import { replaceBadges } from './hightlight-blockquote';
import { html } from '../test-utils/test-input';

describe('highlight util', () => {
  beforeEach(() => {
    document.body.innerHTML = html.slice();
  });

  test('should be defined', () => {
    expect(replaceBadges).toBeDefined();
    expect(typeof replaceBadges).toBe('function');
  });

  test('should not trow an error', () => {
    expect(() => replaceBadges(document.body)).not.toThrow();
  });

  test('should replace [!IMPORTANT] node', () => {
    const selector = '.markdown-alert-important';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');
    const img = p?.querySelector('svg');

    expect(img).toBeDefined();
  });

  xtest('should proceed only top level nodes', () => {});

  test(`tag should be modified only if it has marker in first <p> tag on first line`, () => {
    const selector = '.modified';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');

    expect(p?.querySelector('svg')).toBeTruthy();
  });

  test(`tag should be ignored only if it has marker in first <p> tag on first line`, () => {
    const selector = '.not-modified';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');

    expect(p?.querySelector('svg')).not.toBeTruthy();
  });

  test(`<p> is not first child of <blockquote>, so should be ignored`, () => {
    const selector = '.not-modified-p-is-not-first';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);
    const p = element?.querySelector('p');

    expect(p?.querySelector('svg')).not.toBeTruthy();
  });

  test('should ignore the non top level blockquote', () => {
    const selector = '.not-modified-nested';
    replaceBadges(document.body);
    const element = document.body.querySelector(selector);

    expect(element?.classList.length).toBe(1);
  });
});
